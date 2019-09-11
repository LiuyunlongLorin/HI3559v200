/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    sample_vio.c
 * @brief   sample vio function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */

#include "sample_comm.h"
#include "sensor_interface_cfg_params.h"
#if SUPPORT_GYRO
#include "hi_comm_motionfusion.h"
#include "hi_comm_motionsensor.h"
#include "mpi_motionfusion.h"
#include "motionsensor_chip_cmd.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

static HI_CHAR g_sampleId;
static HI_BOOL g_bDispInited = HI_FALSE;
static HI_BOOL g_bHdmiStarted = HI_FALSE;

#define SAMPLE_MAPI_AI_EXTERN_DEV 0 /** dev for extern acodec */
#define SAMPLE_MAPI_AI_INNER_DEV  0 /** dev for inner acodec */
#define SAMPLE_MAPI_AO_HDMI_DEV   1 /** ao hdmi dev id */

static HI_HANDLE g_acapHdl = SAMPLE_MAPI_AI_INNER_DEV;
static HI_HANDLE g_acapChnHdl = SAMPLE_MAPI_AI_CHN;
static HI_HANDLE g_aoHdl = SAMPLE_MAPI_AO_HDMI_DEV;

static pthread_t g_sendFrmAOChnThd = HI_NULL;
static pthread_t g_hdmiEventProThd = HI_NULL;

static HI_BOOL g_bAoStartFlg = HI_FALSE;
static HI_BOOL g_bHdmiEventProFlg = HI_FALSE;

static HI_S32 g_hdmiEvent = 0;
#define HOTPLUGEVENT 1
#define NOPLUGEVENT  2

HI_VOID SendFrameToAOChnThread(HI_VOID *args)
{
    HI_S32 s32Ret;
    HI_S32 i = 0;
    HI_HANDLE hAcapHdl = g_acapHdl;
    HI_HANDLE hAcapChnHdl = g_acapChnHdl;
    HI_HANDLE hAoHdl = g_aoHdl;
    AUDIO_FRAME_S stFrm;
    AEC_FRAME_S stAecFrm;

    prctl(PR_SET_NAME, (unsigned long)"Hi_SendFrameToAOChnThread", 0, 0, 0);

    while (g_bAoStartFlg) {
        s32Ret = HI_MAPI_ACAP_GetFrame(hAcapHdl, hAcapChnHdl, &stFrm, &stAecFrm);

        if (s32Ret != HI_SUCCESS) {
            usleep(1000 * 5);
            continue;
        }

        i = 0;

        while (i < 10) {
            i++;
            s32Ret = HI_MAPI_AO_SendFrame(hAoHdl, &stFrm, 500);

            if (s32Ret == HI_SUCCESS) {
                break;
            }

            usleep(1000 * 5);
            continue;
        }

        if (10 == i) {
            printf("----------------drop a frame!!!\n");
        }

        s32Ret = HI_MAPI_ACAP_ReleaseFrame(hAcapHdl, hAcapChnHdl, &stFrm, &stAecFrm);
        if (s32Ret != HI_SUCCESS) {
            printf("------HI_MAPI_ACAP_ReleaseFrame error :0x%x!!!\n", s32Ret);
        }
        usleep(1000 * 5);
    }

    return;
}

HI_S32 SAMPLE_GetWindowSize(const VO_PUB_ATTR_S *pstPubAttr, HI_U32 *pu32Width, HI_U32 *pu32Height)
{
    HI_U32 u32Width = 0;
    HI_U32 u32Height = 0;

    switch (pstPubAttr->enIntfSync) {
        case VO_OUTPUT_PAL:
            u32Width = 720;
            u32Height = 576;
            break;

        case VO_OUTPUT_NTSC:
            u32Width = 720;
            u32Height = 480;
            break;

        case VO_OUTPUT_1080P24:
            u32Width = 1920;
            u32Height = 1080;
            break;

        case VO_OUTPUT_1080P25:
            u32Width = 1920;
            u32Height = 1080;
            break;

        case VO_OUTPUT_1080P30:
            u32Width = 1920;
            u32Height = 1080;
            break;

        case VO_OUTPUT_720P50:
            u32Width = 1280;
            u32Height = 720;
            break;

        case VO_OUTPUT_720P60:
            u32Width = 1280;
            u32Height = 720;
            break;

        case VO_OUTPUT_1080I50:
            u32Width = 1920;
            u32Height = 1080;
            break;

        case VO_OUTPUT_1080I60:
            u32Width = 1920;
            u32Height = 1080;
            break;

        case VO_OUTPUT_1080P50:
            u32Width = 1920;
            u32Height = 1080;
            break;

        case VO_OUTPUT_1080P60:
            u32Width = 1920;
            u32Height = 1080;
            break;

        case VO_OUTPUT_576P50:
            u32Width = 720;
            u32Height = 576;
            break;

        case VO_OUTPUT_480P60:
            u32Width = 720;
            u32Height = 480;
            break;

        case VO_OUTPUT_800x600_60:
            u32Width = 800;
            u32Height = 600;
            break;

        case VO_OUTPUT_1024x768_60:
            u32Width = 1024;
            u32Height = 768;
            break;

        case VO_OUTPUT_1280x1024_60:
            u32Width = 1280;
            u32Height = 1024;
            break;

        case VO_OUTPUT_1366x768_60:
            u32Width = 1366;
            u32Height = 768;
            break;

        case VO_OUTPUT_1440x900_60:
            u32Width = 1440;
            u32Height = 900;
            break;

        case VO_OUTPUT_1280x800_60:
            u32Width = 1280;
            u32Height = 800;
            break;

        case VO_OUTPUT_1600x1200_60:
            u32Width = 1600;
            u32Height = 1200;
            break;

        case VO_OUTPUT_1680x1050_60:
            u32Width = 1680;
            u32Height = 1050;
            break;

        case VO_OUTPUT_1920x1200_60:
            u32Width = 1920;
            u32Height = 1200;
            break;

        case VO_OUTPUT_640x480_60:
            u32Width = 640;
            u32Height = 480;
            break;

        case VO_OUTPUT_960H_PAL:
            u32Width = 960;
            u32Height = 576;
            break;

        case VO_OUTPUT_960H_NTSC:
            u32Width = 960;
            u32Height = 480;
            break;

        case VO_OUTPUT_1920x2160_30:
            u32Width = 1920;
            u32Height = 2160;
            break;

        case VO_OUTPUT_2560x1440_30:
            u32Width = 2560;
            u32Height = 1440;
            break;

        case VO_OUTPUT_2560x1600_60:
            u32Width = 2560;
            u32Height = 1600;
            break;

        case VO_OUTPUT_3840x2160_24:
            u32Width = 3840;
            u32Height = 2160;
            break;

        case VO_OUTPUT_3840x2160_25:
            u32Width = 3840;
            u32Height = 2160;
            break;

        case VO_OUTPUT_3840x2160_30:
            u32Width = 3840;
            u32Height = 2160;
            break;

        case VO_OUTPUT_3840x2160_50:
            u32Width = 3840;
            u32Height = 2160;
            break;

        case VO_OUTPUT_3840x2160_60:
            u32Width = 3840;
            u32Height = 2160;
            break;

        case VO_OUTPUT_4096x2160_24:
            u32Width = 4096;
            u32Height = 2160;
            break;

        case VO_OUTPUT_4096x2160_25:
            u32Width = 4096;
            u32Height = 2160;
            break;

        case VO_OUTPUT_4096x2160_30:
            u32Width = 4096;
            u32Height = 2160;
            break;

        case VO_OUTPUT_4096x2160_50:
            u32Width = 4096;
            u32Height = 2160;
            break;

        case VO_OUTPUT_4096x2160_60:
            u32Width = 4096;
            u32Height = 2160;
            break;

        case VO_OUTPUT_320x240_60:
            u32Width = 320;
            u32Height = 240;
            break;

        case VO_OUTPUT_320x240_50:
            u32Width = 320;
            u32Height = 240;
            break;

        case VO_OUTPUT_240x320_50:
            u32Width = 240;
            u32Height = 320;
            break;

        case VO_OUTPUT_240x320_60:
            u32Width = 240;
            u32Height = 320;
            break;

        case VO_OUTPUT_800x600_50:
            u32Width = 800;
            u32Height = 600;
            break;

        case VO_OUTPUT_720x1280_60:
            u32Width = 720;
            u32Height = 1280;
            break;

        case VO_OUTPUT_1080x1920_60:
            u32Width = 1080;
            u32Height = 1920;
            break;

        case VO_OUTPUT_7680x4320_30:
            u32Width = 7680;
            u32Height = 4320;
            break;

        case VO_OUTPUT_USER:
            u32Width = pstPubAttr->stSyncInfo.u16Hact;  // stSyncInfo.u16Hact;
            u32Height = (pstPubAttr->stSyncInfo.bIop) ? pstPubAttr->stSyncInfo.u16Vact :
                pstPubAttr->stSyncInfo.u16Vact * 2;
            break;

        default:
            printf("sample_GetwindowSize ERR\n");
            break;
    }

    *pu32Width = u32Width;
    *pu32Height = u32Height;

    return HI_SUCCESS;
}
HI_VOID HdmiEventProThread(HI_VOID *args)
{
#ifdef SUPPORT_HDMI

    HI_S32 s32Ret;
    HI_S32 i = 0;
    HI_HANDLE hDispHdl0 = 0;
    HI_HANDLE hWndHdl0 = 0;
    HI_MAPI_DISP_ATTR_S stDispAttr;
    HI_MAPI_DISP_VIDEOLAYER_ATTR_S stVideoLayerAttr;
    HI_MAPI_DISP_WINDOW_ATTR_S stWndAttr;
    HI_U32 u32WinWidth;
    HI_U32 u32WinHeight;

    HI_HANDLE hVpssHdl0 = 0;
    HI_HANDLE hVPortHdl0 = 0;
    HI_BOOL bStitch = HI_FALSE;

    HI_HANDLE hAcapHdl = SAMPLE_MAPI_AI_INNER_DEV;
    HI_HANDLE hAcapChnHdl = SAMPLE_MAPI_AI_CHN;
    HI_HANDLE hAoHdl = SAMPLE_MAPI_AO_HDMI_DEV;
    AUDIO_SAMPLE_RATE_E enHdmiSamRate = AUDIO_SAMPLE_RATE_BUTT;
    HI_MAPI_ACAP_ATTR_S stACapAttr;
    HI_MAPI_AO_ATTR_S stAoAttr;
    HI_S32 s32InGain;
    HI_S32 s32OutGain;

    HI_HDMI_ID_E enHdmi = HI_HDMI_ID_0;
    HI_MAPI_HDMI_SINKCAPABILITY_S stsinkcapability;
    HI_MAPI_HDMI_ATTR_S stHdmiAttr;

    prctl(PR_SET_NAME, (unsigned long)"hdmi event pro Thread", 0, 0, 0);

    while (g_bHdmiEventProFlg) {
        if (g_hdmiEvent) {
            switch (g_hdmiEvent) {
                case HOTPLUGEVENT:

                    /* get sink capbility */
                    CHECK_RET(HI_MAPI_HDMI_GetSinkCapability(enHdmi, &stsinkcapability));

                    if (!(stsinkcapability.bConnected && stsinkcapability.bIsSinkPowerOn)) {
                        g_hdmiEvent = 0;
                        break;
                    }

                    /* If you have initialized disp you need to deinitialize */
                    if (g_bDispInited) {
                        CHECK_RET(HI_MAPI_DISP_UnBind_VProc(hVpssHdl0, hVPortHdl0, hDispHdl0, hWndHdl0, bStitch));
                        CHECK_RET(HI_MAPI_DISP_StopWindow(hDispHdl0, hWndHdl0));
                        CHECK_RET(HI_MAPI_DISP_Stop(hDispHdl0));
                        CHECK_RET(HI_MAPI_DISP_Deinit(hDispHdl0));
                        g_bDispInited = HI_FALSE;
                    }

                    /* config disp */
                    if (stsinkcapability.enNativeIntfSync != VO_OUTPUT_BUTT) {
                        stDispAttr.stPubAttr.enIntfSync = stsinkcapability.enNativeIntfSync;
                    } else {
                        for (i = 0; i < VO_OUTPUT_BUTT; i++) {
                            if (stsinkcapability.abIntfSync[i] == HI_TRUE) {
                                stDispAttr.stPubAttr.enIntfSync = (VO_INTF_SYNC_E)i;
                                break;
                            }
                        }

                        if (i == VO_OUTPUT_BUTT) {
                            stDispAttr.stPubAttr.enIntfSync = VO_OUTPUT_1080P30;
                        }
                    }

                    stDispAttr.stPubAttr.u32BgColor = 0xFF;
                    stDispAttr.stPubAttr.enIntfType = VO_INTF_HDMI;
                    memset(&stDispAttr.stPubAttr.stSyncInfo, 0, sizeof(VO_SYNC_INFO_S));

                    SAMPLE_GetWindowSize(&stDispAttr.stPubAttr, &u32WinWidth, &u32WinHeight);
                    stWndAttr.stRect.s32X = 0;
                    stWndAttr.stRect.s32Y = 0;
                    stWndAttr.stRect.u32Width = u32WinWidth;
                    stWndAttr.stRect.u32Height = u32WinHeight;
                    stWndAttr.u32Priority = 0;
                    memset(&stVideoLayerAttr, 0, sizeof(HI_MAPI_DISP_VIDEOLAYER_ATTR_S));
                    stVideoLayerAttr.u32BufLen = 4;

                    CHECK_RET(HI_MAPI_DISP_Init(hDispHdl0, &stDispAttr));
                    CHECK_RET(HI_MAPI_DISP_Start(hDispHdl0, &stVideoLayerAttr));
                    CHECK_RET(HI_MAPI_DISP_SetWindowAttr(hDispHdl0, hWndHdl0, &stWndAttr));
                    CHECK_RET(HI_MAPI_DISP_StartWindow(hDispHdl0, hWndHdl0));
                    CHECK_RET(HI_MAPI_DISP_Bind_VProc(hVpssHdl0, hVPortHdl0, hDispHdl0, hWndHdl0, bStitch));
                    g_bDispInited = HI_TRUE;

                    /* configer audio */
                    if (stsinkcapability.bSupportHdmi && (stsinkcapability.aenSupportSampleRate[0] !=
                        AUDIO_SAMPLE_RATE_BUTT)) {
                        /* If you have initialized audio you need to deinitialize */
                        if (g_bAoStartFlg) {
                            g_bAoStartFlg = HI_FALSE;
                            pthread_join(g_sendFrmAOChnThd, HI_NULL);
                            CHECK_RET(HI_MAPI_ACAP_Stop(hAcapHdl, hAcapChnHdl));
                            CHECK_RET(HI_MAPI_ACAP_Deinit(hAcapHdl));
                            CHECK_RET(HI_MAPI_AO_Stop(hAoHdl));
                            CHECK_RET(HI_MAPI_AO_Deinit(hAoHdl));
                        }

                        /* start acap */
                        enHdmiSamRate = stsinkcapability.aenSupportSampleRate[0];

                        for (i = 0; i < HI_MAPI_AO_MAX_SUPPORT_SAMPLERATE_NUM; i++) { //  prior choose 48000
                            if (AUDIO_SAMPLE_RATE_48000 == stsinkcapability.aenSupportSampleRate[i]) {
                                enHdmiSamRate = stsinkcapability.aenSupportSampleRate[i];
                                break;
                            }
                        }

                        stACapAttr.enWorkmode = AIO_MODE_I2S_MASTER;
                        stACapAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
                        stACapAttr.enSampleRate = enHdmiSamRate;
                        stACapAttr.enSoundMode = HI_MAPI_AUDIO_SOUND_MODE_STEREO;
                        stACapAttr.u32PtNumPerFrm = 1024;
                        stACapAttr.enMixerMicMode = ACODEC_MIXER_IN1;
                        stACapAttr.enResampleRate = AUDIO_SAMPLE_RATE_48000;
                        stACapAttr.enI2sType = AIO_I2STYPE_INNERCODEC;
                        s32InGain = 20;

                        CHECK_RET(HI_MAPI_ACAP_Init(hAcapHdl, &stACapAttr));
                        CHECK_RET(HI_MAPI_ACAP_Start(hAcapHdl, hAcapChnHdl));
                        CHECK_RET(HI_MAPI_ACAP_SetVolume(hAcapHdl, s32InGain));

                        /* start ao */
                        stAoAttr.enSampleRate = enHdmiSamRate;
                        stAoAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
                        stAoAttr.enSoundMode = HI_MAPI_AUDIO_SOUND_MODE_STEREO;
                        stAoAttr.enWorkmode = AIO_MODE_I2S_MASTER;
                        stAoAttr.u32PtNumPerFrm = 1024;
                        stAoAttr.enInSampleRate = AUDIO_SAMPLE_RATE_48000;
                        stAoAttr.enI2sType = AIO_I2STYPE_INNERHDMI;
                        s32OutGain = 0;

                        CHECK_RET(HI_MAPI_AO_Init(hAoHdl, &stAoAttr));
                        CHECK_RET(HI_MAPI_AO_Start(hAoHdl));
                        CHECK_RET(HI_MAPI_AO_SetVolume(hAoHdl, s32OutGain));

                        /* acap send frame to ao */
                        g_bAoStartFlg = HI_TRUE;
                        s32Ret = pthread_create(&g_sendFrmAOChnThd, HI_NULL, (HI_VOID *)SendFrameToAOChnThread, HI_NULL);
                        if (s32Ret != HI_SUCCESS) {
                            g_bAoStartFlg = HI_FALSE;
                            CHECK_RET(HI_MAPI_ACAP_Stop(hAcapHdl, hAcapChnHdl));
                            CHECK_RET(HI_MAPI_ACAP_Deinit(hAcapHdl));
                            CHECK_RET(HI_MAPI_AO_Stop(hAoHdl));
                            CHECK_RET(HI_MAPI_AO_Deinit(hAoHdl));
                        }
                    }

                    /* config hdmi */
                    stHdmiAttr.bEnableHdmi = stsinkcapability.bSupportHdmi;

                    if (g_bAoStartFlg) {
                        stHdmiAttr.bEnableAudio = HI_TRUE;
                    } else {
                        stHdmiAttr.bEnableAudio = HI_FALSE;
                    }

                    stHdmiAttr.bEnableVideo = HI_TRUE;
                    stHdmiAttr.enHdmiSync = stDispAttr.stPubAttr.enIntfSync;
                    stHdmiAttr.enSampleRate = enHdmiSamRate;
                    stHdmiAttr.u32PixClk = 0;

                    CHECK_RET(HI_MAPI_HDMI_SetAttr(enHdmi, &stHdmiAttr));
                    CHECK_RET(HI_MAPI_HDMI_Start(enHdmi));
                    g_bHdmiStarted = HI_TRUE;
                    g_hdmiEvent = 0;

                    break;

                case NOPLUGEVENT:

                    /* deinit ao */
                    if (g_bAoStartFlg) {
                        g_bAoStartFlg = HI_FALSE;
                        pthread_join(g_sendFrmAOChnThd, HI_NULL);
                        CHECK_RET(HI_MAPI_ACAP_Stop(hAcapHdl, hAcapChnHdl));
                        CHECK_RET(HI_MAPI_ACAP_Deinit(hAcapHdl));
                        CHECK_RET(HI_MAPI_AO_Stop(hAoHdl));
                        CHECK_RET(HI_MAPI_AO_Deinit(hAoHdl));
                    }

                    /* stop HDMI */
                    if (g_bHdmiStarted) {
                        CHECK_RET(HI_MAPI_HDMI_Stop(enHdmi));
                        g_bHdmiStarted = HI_FALSE;
                    }

                    /* stop DISP */
                    if (g_bDispInited) {
                        CHECK_RET(HI_MAPI_DISP_UnBind_VProc(hVpssHdl0, hVPortHdl0, hDispHdl0, hWndHdl0, bStitch));
                        CHECK_RET(HI_MAPI_DISP_StopWindow(hDispHdl0, hWndHdl0));
                        CHECK_RET(HI_MAPI_DISP_Stop(hDispHdl0));
                        CHECK_RET(HI_MAPI_DISP_Deinit(hDispHdl0));
                        g_bDispInited = HI_FALSE;
                    }

                    /* config disp */
                    stDispAttr.stPubAttr.u32BgColor = 0xFF;
                    stDispAttr.stPubAttr.enIntfType = VO_INTF_BT1120;
                    stDispAttr.stPubAttr.enIntfSync = VO_OUTPUT_1080P30;
                    memset(&stDispAttr.stPubAttr.stSyncInfo, 0, sizeof(VO_SYNC_INFO_S));

                    stWndAttr.stRect.s32X = 0;
                    stWndAttr.stRect.s32Y = 0;
                    stWndAttr.stRect.u32Width = 1920;
                    stWndAttr.stRect.u32Height = 1080;
                    stWndAttr.u32Priority = 0;
                    HI_BOOL bStitch = HI_FALSE;
                    memset(&stVideoLayerAttr, 0, sizeof(HI_MAPI_DISP_VIDEOLAYER_ATTR_S));
                    stVideoLayerAttr.u32BufLen = 4;

                    CHECK_RET(HI_MAPI_DISP_Init(hDispHdl0, &stDispAttr));
                    CHECK_RET(HI_MAPI_DISP_Start(hDispHdl0, &stVideoLayerAttr));
                    CHECK_RET(HI_MAPI_DISP_SetWindowAttr(hDispHdl0, hWndHdl0, &stWndAttr));
                    CHECK_RET(HI_MAPI_DISP_StartWindow(hDispHdl0, hWndHdl0));
                    CHECK_RET(HI_MAPI_DISP_Bind_VProc(hVpssHdl0, hVPortHdl0, hDispHdl0, hWndHdl0, bStitch));
                    g_bDispInited = HI_TRUE;

                    g_hdmiEvent = 0;
                    break;

                default:
                    break;
            }
        }
    }

exit:

    if (g_bAoStartFlg) {
        g_bAoStartFlg = HI_FALSE;
        pthread_join(g_sendFrmAOChnThd, HI_NULL);
        HI_MAPI_ACAP_Stop(hAcapHdl, hAcapChnHdl);
        HI_MAPI_ACAP_Deinit(hAcapHdl);
        HI_MAPI_AO_Stop(hAoHdl);
        HI_MAPI_AO_Deinit(hAoHdl);
    }
#endif
    return;
}
#ifdef SUPPORT_HDMI
HI_VOID SAMPLE_VIO_HdmiEventUpload(HI_HDMI_EVENT_TYPE_E event, HI_VOID *pPrivateData)
{
    switch (event) {
        case HI_HDMI_EVENT_HOTPLUG:
            g_hdmiEvent = HOTPLUGEVENT;
            printf("\033[0;32mhdmi HOTPLUG event! \033[0;39m\n");

            break;

        case HI_HDMI_EVENT_NO_PLUG:
            g_hdmiEvent = NOPLUGEVENT;
            printf("\033[0;31mhdmi NO_PLUG event! \033[0;39m\n");
            break;

        default:
            printf("\033[0;31minvalid hdmi event! \033[0;39m\n");
            break;
    }
}
#endif

HI_VOID SAMPLE_VIO_Usage()
{
    printf("Usage : \n");
    printf("\t 0) VIO HDMI Vieo preview.\n");
#ifdef SUPPORT_HDMI
    printf("\t 1) VIO HMDI Vieo+Audio.\n");
    printf("\t 2) VIO HMDI AutoAdapt.\n");
#endif
#ifdef SUPPORT_GYRO
    printf("\t 3) VIO HDMI preview in gyro mode.\n");
#endif
    printf("\t 4) VIO HDMI preview with ahd.\n");
    printf("\t q/Q) quit\n");
    return;
}

HI_S32 SAMPLE_VIO_HDMI_Preview()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8 u8SnsCnt = 1;

    /**************************start vcap *******************************/
#ifndef SUPPORT_HDMI
    printf("not support HDMI,not display normally in this test!\n");
#endif
    HI_HANDLE hVcapDev0 = VCAP_DEV_0;
    HI_HANDLE hVcapPipe0 = VCAP_PIPE_0;
    HI_HANDLE hPipeChn0 = PIPE_CHN_0;

    HI_MAPI_VCAP_ATTR_S stVcapAttr;
    HI_MAPI_SENSOR_ATTR_S stVcapSensorAttr;
    SAMPLE_COMM_INITATTR_S stSampleCommAttr;
    SAMPLE_MODE_COMM_CONFIG_S stCommConfig;
    SAMPLE_SENSOR_SEQ_E enSensorSeq = SAMPLE_SENSOR_1080P30;

    /* sensor0 and media cfg */
    CHECK_RET(SAMPLE_VENC_Get_Media_Mode(&enSensorSeq, &stVcapSensorAttr));
    CHECK_RET(MAPI_SAMPLE_COMM_GetMediaCFG(enSensorSeq, &stCommConfig));
    if (enSensorSeq == SAMPLE_SENSOR_4K30) {
        printf("\n----4K@30fps single PIPE Video + 1080p HDMI preview----\n\n");
    } else if (enSensorSeq == SAMPLE_SENSOR_4M30) {
        printf("\n----4M@30fps single PIPE Video + 1080p HDMI preview----\n\n");
    } else {
        printf("\n----1080p@30fps  single PIPE Video + 1080p HDMI preview----\n\n");
    }

    /* dev0 attr */
    memcpy(&stVcapAttr.stVcapDevAttr, stCommConfig.pstVcapDevAttr, sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
    stVcapAttr.u32PipeBindNum = 1;
    stVcapAttr.aPipeIdBind[0] = hVcapPipe0;

    /* Pipe0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0], stCommConfig.pstVcapPipeAttr, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /* Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0].astPipeChnAttr[hPipeChn0], stCommConfig.pstPipeChnAttr,
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    /* Pipe2 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[VCAP_PIPE_2], stCommConfig.pstVcapPipeAttr, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[VCAP_PIPE_2].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /* Pipe2 Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[VCAP_PIPE_2].astPipeChnAttr[hPipeChn0], stCommConfig.pstPipeChnAttr,
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    /* VB and media Attr */
    memset(&stSampleCommAttr, 0, sizeof(SAMPLE_COMM_INITATTR_S));
    memset(&stSampleCommAttr.stViVpssMode, 0, sizeof(VI_VPSS_MODE_S));
    stSampleCommAttr.stResolution.u32Width = stVcapSensorAttr.stSize.u32Width;
    stSampleCommAttr.stResolution.u32Height = stVcapSensorAttr.stSize.u32Height;
    stSampleCommAttr.stViVpssMode.aenMode[hVcapPipe0] = VI_OFFLINE_VPSS_OFFLINE;
    stSampleCommAttr.u8SnsCnt = u8SnsCnt;

    /* init VB and media */
    CHECK_RET(SAMPLE_COMM_Init(stSampleCommAttr, VB_MODE_1));

    /* vcap dev0 */
    CHECK_RET(HI_MAPI_VCAP_InitSensor(hVcapDev0, &stVcapSensorAttr));
    CHECK_RET(HI_MAPI_VCAP_SetAttr(hVcapDev0, &stVcapAttr));
    CHECK_RET(HI_MAPI_VCAP_StartDev(hVcapDev0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe0, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StartISP(hVcapPipe0));

    /* vcap dev1 only  for reference with two sensor . */
    if ((IMX307 == CFG_SENSOR_TYPE0) && (IMX307 == CFG_SENSOR_TYPE1)) {
        stVcapAttr.aPipeIdBind[0] = VCAP_PIPE_2;
        CHECK_RET(HI_MAPI_VCAP_InitSensor(VCAP_DEV_1, &stVcapSensorAttr));
        CHECK_RET(HI_MAPI_VCAP_SetAttr(VCAP_DEV_1, &stVcapAttr));
        CHECK_RET(HI_MAPI_VCAP_StartDev(VCAP_DEV_1));
        CHECK_RET(HI_MAPI_VCAP_StartChn(VCAP_PIPE_2, PIPE_CHN_0));
        CHECK_RET(HI_MAPI_VCAP_StartISP(VCAP_PIPE_2));
    }

    /**************************start vproc *******************************/
    HI_HANDLE hVpssHdl0 = 0;
    HI_HANDLE hVPortHdl0 = 0;
    HI_BOOL bStitch = HI_FALSE;
    HI_MAPI_VPSS_ATTR_S stVpssAttr;
    HI_MAPI_VPORT_ATTR_S stVPortAttr;

    /* Vpss0 attr */
    memcpy(&stVpssAttr, stCommConfig.pstVpssAttr, sizeof(HI_MAPI_VPSS_ATTR_S));

    /* VPort0 attr */
    memcpy(&stVPortAttr, stCommConfig.pstVPortAttr, sizeof(HI_MAPI_VPORT_ATTR_S));

    CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl0, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl0, hVPortHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl0, hVPortHdl0));

    /**************************start preview *******************************/
    CHECK_RET(SAMPLE_COMM_StartPreview(hVpssHdl0, hVPortHdl0, bStitch));

    printf("Press Enter key to stop preview...\n");
    (void)getchar();

exit:

    /* stop preview */
    SAMPLE_COMM_StopPreview(hVpssHdl0, hVPortHdl0, bStitch);

    /* stop vproc 0 */
    HI_MAPI_VPROC_StopPort(hVpssHdl0, hVPortHdl0);
    HI_MAPI_VPROC_UnBindVCap(hVcapPipe0, hPipeChn0, hVpssHdl0);
    HI_MAPI_VPROC_DeinitVpss(hVpssHdl0);

    /* stop vcap 0 */
    HI_MAPI_VCAP_StopISP(hVcapPipe0);
    HI_MAPI_VCAP_StopChn(hVcapPipe0, hPipeChn0);
    HI_MAPI_VCAP_StopDev(hVcapDev0);
    HI_MAPI_VCAP_DeinitSensor(hVcapDev0);

    /* vcap dev1 only  for reference with two sensor . */
    if ((IMX307 == CFG_SENSOR_TYPE0) && (IMX307 == CFG_SENSOR_TYPE1)) {
        HI_MAPI_VCAP_StopISP(VCAP_PIPE_2);
        HI_MAPI_VCAP_StopChn(VCAP_PIPE_2, hPipeChn0);
        HI_MAPI_VCAP_StopDev(hVcapDev0);
        HI_MAPI_VCAP_DeinitSensor(hVcapDev0);
    }
    /* stop media */
    SAMPLE_COMM_Deinit();

    return s32Ret;
}

HI_S32 SAMPLE_HDMI_AudioOutput()
{
    HI_S32 s32Ret;
#ifdef SUPPORT_HDMI
    HI_U8 u8SnsCnt = 1;

    /**************************start vcap *******************************/

    HI_HANDLE hVcapDev0 = VCAP_DEV_0;
    HI_HANDLE hVcapPipe0 = VCAP_PIPE_0;
    HI_HANDLE hPipeChn0 = PIPE_CHN_0;

    HI_MAPI_VCAP_ATTR_S stVcapAttr;
    HI_MAPI_DISP_VIDEOLAYER_ATTR_S stVideoLayerAttr;
    HI_MAPI_SENSOR_ATTR_S stVcapSensorAttr;
    SAMPLE_COMM_INITATTR_S stSampleCommAttr;

    printf("\n----4K@30fps single PIPE +  HDMI Video+Audio ----\n\n");

    /* sensor0 attr */
    CHECK_RET(SAMPLE_COMM_GetSensorCfg(SAMPLE_SENSOR_4K30, &stVcapSensorAttr));

    /* dev0 attr */
    memcpy(&stVcapAttr.stVcapDevAttr, &g_stDev_4K, sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
    stVcapAttr.u32PipeBindNum = 1;
    stVcapAttr.aPipeIdBind[0] = hVcapPipe0;

    /* Pipe0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0], &g_stPipe_4K30FPS, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /* Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0].astPipeChnAttr[hPipeChn0], &g_stChn_4K30FPS,
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    /* VB and media Attr */
    memset(&stSampleCommAttr, 0, sizeof(SAMPLE_COMM_INITATTR_S));
    memset(&stSampleCommAttr.stViVpssMode, 0, sizeof(VI_VPSS_MODE_S));
    stSampleCommAttr.stResolution.u32Width = stVcapSensorAttr.stSize.u32Width;
    stSampleCommAttr.stResolution.u32Height = stVcapSensorAttr.stSize.u32Height;
    stSampleCommAttr.stViVpssMode.aenMode[hVcapPipe0] = VI_OFFLINE_VPSS_OFFLINE;
    stSampleCommAttr.u8SnsCnt = u8SnsCnt;

    /* init VB and media */
    CHECK_RET(SAMPLE_COMM_Init(stSampleCommAttr, VB_MODE_1));

    /* vcap */
    CHECK_RET(HI_MAPI_VCAP_InitSensor(hVcapDev0, &stVcapSensorAttr));
    CHECK_RET(HI_MAPI_VCAP_SetAttr(hVcapDev0, &stVcapAttr));
    CHECK_RET(HI_MAPI_VCAP_StartDev(hVcapDev0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe0, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StartISP(hVcapPipe0));

    /**************************start vproc *******************************/
    HI_HANDLE hVpssHdl0 = 0;
    HI_HANDLE hVPortHdl0 = 0;
    HI_BOOL bStitch = HI_FALSE;
    HI_MAPI_VPSS_ATTR_S stVpssAttr;
    HI_MAPI_VPORT_ATTR_S stVPortAttr;

    /* Vpss0 attr */
    memcpy(&stVpssAttr, &g_stVpss_4K30FPS, sizeof(HI_MAPI_VPSS_ATTR_S));

    /* VPort0 attr */
    memcpy(&stVPortAttr, &g_stVport_4K30FPS, sizeof(HI_MAPI_VPORT_ATTR_S));

    CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl0, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl0, hVPortHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl0, hVPortHdl0));

    /**************************start preview *******************************/
    /* start disp */
    HI_HANDLE hDispHdl0 = 0;
    HI_HANDLE hWndHdl0 = 0;
    HI_MAPI_DISP_ATTR_S stDispAttr;
    HI_MAPI_DISP_WINDOW_ATTR_S stWndAttr;

    /* Disp0 attr */
    stDispAttr.stPubAttr.u32BgColor = 0xFF;
    stDispAttr.stPubAttr.enIntfType = VO_INTF_HDMI;
    stDispAttr.stPubAttr.enIntfSync = VO_OUTPUT_1080P60;
    memset(&stDispAttr.stPubAttr.stSyncInfo, 0, sizeof(VO_SYNC_INFO_S));

    /* hWndHdl0 attr */
    stWndAttr.stRect.s32X = 0;
    stWndAttr.stRect.s32Y = 0;
    stWndAttr.stRect.u32Width = 1920;
    stWndAttr.stRect.u32Height = 1080;
    stWndAttr.u32Priority = 0;
    memset(&stVideoLayerAttr, 0, sizeof(HI_MAPI_DISP_VIDEOLAYER_ATTR_S));
    stVideoLayerAttr.u32BufLen = 4;

    CHECK_RET(HI_MAPI_DISP_Init(hDispHdl0, &stDispAttr));
    CHECK_RET(HI_MAPI_DISP_Start(hDispHdl0, &stVideoLayerAttr));
    CHECK_RET(HI_MAPI_DISP_SetWindowAttr(hDispHdl0, hWndHdl0, &stWndAttr));
    CHECK_RET(HI_MAPI_DISP_StartWindow(hDispHdl0, hWndHdl0));
    CHECK_RET(HI_MAPI_DISP_Bind_VProc(hVpssHdl0, hVPortHdl0, hDispHdl0, hWndHdl0, bStitch));

    /**************************start hdmi ao *******************************/
    /* start hdmi ao */
    HI_HANDLE hAcapHdl = SAMPLE_MAPI_AI_INNER_DEV;
    HI_HANDLE hAcapChnHdl = SAMPLE_MAPI_AI_CHN;
    HI_HANDLE hAoHdl = SAMPLE_MAPI_AO_HDMI_DEV;
    HI_MAPI_ACAP_ATTR_S stACapAttr;
    HI_S32 s32InGain;

    /* init and start acap */
    stACapAttr.enWorkmode = AIO_MODE_I2S_MASTER;
    stACapAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
    stACapAttr.enSampleRate = AUDIO_SAMPLE_RATE_48000;
    stACapAttr.enSoundMode = HI_MAPI_AUDIO_SOUND_MODE_STEREO;
    stACapAttr.u32PtNumPerFrm = 1024;
    stACapAttr.enMixerMicMode = ACODEC_MIXER_IN1;
    stACapAttr.enResampleRate = AUDIO_SAMPLE_RATE_48000;
    stACapAttr.enI2sType = AIO_I2STYPE_INNERCODEC;
    s32InGain = 20;

    CHECK_RET(HI_MAPI_ACAP_Init(hAcapHdl, &stACapAttr));
    CHECK_RET(HI_MAPI_ACAP_Start(hAcapHdl, hAcapChnHdl));
    CHECK_RET(HI_MAPI_ACAP_SetVolume(hAcapHdl, s32InGain));

    /* init and start HDMI ao */
    HI_MAPI_AO_ATTR_S stAoAttr;
    HI_S32 s32OutGain;
    stAoAttr.enSampleRate = AUDIO_SAMPLE_RATE_48000;
    stAoAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
    stAoAttr.enSoundMode = HI_MAPI_AUDIO_SOUND_MODE_STEREO;
    stAoAttr.enWorkmode = AIO_MODE_I2S_MASTER;
    stAoAttr.u32PtNumPerFrm = 1024;
    stAoAttr.enInSampleRate = AUDIO_SAMPLE_RATE_48000;
    stAoAttr.enI2sType = AIO_I2STYPE_INNERHDMI;
    s32OutGain = 0;

    CHECK_RET(HI_MAPI_AO_Init(hAoHdl, &stAoAttr));
    CHECK_RET(HI_MAPI_AO_Start(hAoHdl));
    CHECK_RET(HI_MAPI_AO_SetVolume(hAoHdl, s32OutGain));

    /* acap send frame to ao */
    g_bAoStartFlg = HI_TRUE;
    s32Ret = pthread_create(&g_sendFrmAOChnThd, HI_NULL, (HI_VOID *)SendFrameToAOChnThread, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        g_bAoStartFlg = HI_FALSE;
        printf("pthread_create error!...\n");
    }
    /* start Hdmi */
    HI_HDMI_ID_E enHdmi = HI_HDMI_ID_0;
    HI_HDMI_CALLBACK_FUNC_S stHdmiCallbackFun;
    HI_MAPI_HDMI_ATTR_S stHdmiAttr;

    /* Hdmi attr */
    stHdmiCallbackFun.pfnHdmiEventCallback = SAMPLE_COMM_EventPro;
    stHdmiCallbackFun.pPrivateData = HI_NULL;
    stHdmiAttr.bEnableHdmi = HI_TRUE;
    stHdmiAttr.bEnableAudio = HI_TRUE;
    stHdmiAttr.bEnableVideo = HI_TRUE;
    stHdmiAttr.enHdmiSync = stDispAttr.stPubAttr.enIntfSync;
    stHdmiAttr.enSampleRate = AUDIO_SAMPLE_RATE_48000;
    stHdmiAttr.u32PixClk = 0;

    CHECK_RET(HI_MAPI_HDMI_Init(enHdmi, &stHdmiCallbackFun));
    CHECK_RET(HI_MAPI_HDMI_SetAttr(enHdmi, &stHdmiAttr));
    CHECK_RET(HI_MAPI_HDMI_Start(enHdmi));

    printf("Press Enter key to stop preview...\n");
    (void)getchar();

    if (g_bAoStartFlg) {
        g_bAoStartFlg = HI_FALSE;
        pthread_join(g_sendFrmAOChnThd, HI_NULL);
    }

exit:

    /* stop hdmi 0 */
    HI_MAPI_HDMI_Stop(HI_HDMI_ID_0);
    HI_MAPI_HDMI_Deinit(HI_HDMI_ID_0);

    /* stop audio */
    HI_MAPI_ACAP_Stop(hAcapHdl, g_acapChnHdl);
    HI_MAPI_ACAP_Deinit(hAcapHdl);
    HI_MAPI_AO_Stop(hAoHdl);
    HI_MAPI_AO_Deinit(hAoHdl);

    /* stop disp 0 */
    HI_MAPI_DISP_UnBind_VProc(hVpssHdl0, hVPortHdl0, hDispHdl0, hWndHdl0, bStitch);
    HI_MAPI_DISP_StopWindow(hDispHdl0, hWndHdl0);
    HI_MAPI_DISP_Stop(hDispHdl0);
    HI_MAPI_DISP_Deinit(hDispHdl0);

    /* stop vproc 0 */
    HI_MAPI_VPROC_StopPort(hVpssHdl0, hVPortHdl0);
    HI_MAPI_VPROC_UnBindVCap(hVcapPipe0, hPipeChn0, hVpssHdl0);
    HI_MAPI_VPROC_DeinitVpss(hVpssHdl0);

    /* stop vcap 0 */
    HI_MAPI_VCAP_StopISP(hVcapPipe0);
    HI_MAPI_VCAP_StopChn(hVcapPipe0, hPipeChn0);
    HI_MAPI_VCAP_StopDev(hVcapDev0);
    HI_MAPI_VCAP_DeinitSensor(hVcapDev0);

    /* stop media */
    SAMPLE_COMM_Deinit();
#endif
    return HI_SUCCESS;
}

HI_S32 SAMPLE_HDMI_AutoAdapt()
{
    HI_S32 s32Ret;
#ifdef SUPPORT_HDMI
    HI_U8 u8SnsCnt = 1;

    printf("\n----4K@30fps single PIPE + HDMI autoadapt----\n\n");

    /**************************start vcap *******************************/
    HI_HANDLE hVcapDev0 = VCAP_DEV_0;
    HI_HANDLE hVcapPipe0 = VCAP_PIPE_0;
    HI_HANDLE hPipeChn0 = PIPE_CHN_0;

    HI_MAPI_VCAP_ATTR_S stVcapAttr;
    HI_MAPI_SENSOR_ATTR_S stVcapSensorAttr;
    SAMPLE_COMM_INITATTR_S stSampleCommAttr;
    HI_MAPI_DISP_VIDEOLAYER_ATTR_S stVideoLayerAttr;

    /* sensor0 attr */
    CHECK_RET(SAMPLE_COMM_GetSensorCfg(SAMPLE_SENSOR_4K30, &stVcapSensorAttr));

    /* dev0 attr */
    memcpy(&stVcapAttr.stVcapDevAttr, &g_stDev_4K, sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
    stVcapAttr.u32PipeBindNum = 1;
    stVcapAttr.aPipeIdBind[0] = hVcapPipe0;

    /* Pipe0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0], &g_stPipe_4K30FPS, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /* Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0].astPipeChnAttr[hPipeChn0], &g_stChn_4K30FPS,
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    /* VB and media Attr */
    memset(&stSampleCommAttr, 0, sizeof(SAMPLE_COMM_INITATTR_S));
    memset(&stSampleCommAttr.stViVpssMode, 0, sizeof(VI_VPSS_MODE_S));
    stSampleCommAttr.stResolution.u32Width = stVcapSensorAttr.stSize.u32Width;
    stSampleCommAttr.stResolution.u32Height = stVcapSensorAttr.stSize.u32Height;
    stSampleCommAttr.stViVpssMode.aenMode[hVcapPipe0] = VI_OFFLINE_VPSS_OFFLINE;
    stSampleCommAttr.u8SnsCnt = u8SnsCnt;

    /* init VB and media */
    CHECK_RET(SAMPLE_COMM_Init(stSampleCommAttr, VB_MODE_1));

    /* vcap */
    CHECK_RET(HI_MAPI_VCAP_InitSensor(hVcapDev0, &stVcapSensorAttr));
    CHECK_RET(HI_MAPI_VCAP_SetAttr(hVcapDev0, &stVcapAttr));
    CHECK_RET(HI_MAPI_VCAP_StartDev(hVcapDev0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe0, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StartISP(hVcapPipe0));

    /**************************start vproc *******************************/
    HI_HANDLE hVpssHdl0 = 0;
    HI_HANDLE hVPortHdl0 = 0;
    HI_BOOL bStitch = HI_FALSE;
    HI_MAPI_VPSS_ATTR_S stVpssAttr;
    HI_MAPI_VPORT_ATTR_S stVPortAttr;

    /* Vpss0 attr */
    memcpy(&stVpssAttr, &g_stVpss_4K30FPS, sizeof(HI_MAPI_VPSS_ATTR_S));

    /* VPort0 attr */
    memcpy(&stVPortAttr, &g_stVport_4K30FPS, sizeof(HI_MAPI_VPORT_ATTR_S));

    CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl0, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl0, hVPortHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl0, hVPortHdl0));

    /**************************start preview *******************************/
    HI_HANDLE hDispHdl0 = 0;
    HI_HANDLE hWndHdl0 = 0;
    HI_MAPI_DISP_ATTR_S stDispAttr;
    HI_MAPI_DISP_WINDOW_ATTR_S stWndAttr;

    /* Disp0 attr */
    stDispAttr.stPubAttr.u32BgColor = 0xFF;
    stDispAttr.stPubAttr.enIntfType = VO_INTF_BT1120;
    stDispAttr.stPubAttr.enIntfSync = VO_OUTPUT_1080P30;
    memset(&stDispAttr.stPubAttr.stSyncInfo, 0, sizeof(VO_SYNC_INFO_S));

    /* hWndHdl0 attr */
    stWndAttr.stRect.s32X = 0;
    stWndAttr.stRect.s32Y = 0;
    stWndAttr.stRect.u32Width = 1920;
    stWndAttr.stRect.u32Height = 1080;
    stWndAttr.u32Priority = 0;
    memset(&stVideoLayerAttr, 0, sizeof(HI_MAPI_DISP_VIDEOLAYER_ATTR_S));
    stVideoLayerAttr.u32BufLen = 4;

    CHECK_RET(HI_MAPI_DISP_Init(hDispHdl0, &stDispAttr));
    CHECK_RET(HI_MAPI_DISP_Start(hDispHdl0, &stVideoLayerAttr));
    CHECK_RET(HI_MAPI_DISP_SetWindowAttr(hDispHdl0, hWndHdl0, &stWndAttr));
    CHECK_RET(HI_MAPI_DISP_StartWindow(hDispHdl0, hWndHdl0));
    CHECK_RET(HI_MAPI_DISP_Bind_VProc(hVpssHdl0, hVPortHdl0, hDispHdl0, hWndHdl0, bStitch));
    g_bDispInited = HI_TRUE;

    g_bHdmiEventProFlg = HI_TRUE;
    s32Ret = pthread_create(&g_hdmiEventProThd, HI_NULL, (HI_VOID *)HdmiEventProThread, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        g_bHdmiEventProFlg = HI_FALSE;
    }

    /* start Hdmi */
    HI_HDMI_ID_E enHdmi = HI_HDMI_ID_0;
    HI_HDMI_CALLBACK_FUNC_S stHdmiCallbackFun;

    stHdmiCallbackFun.pfnHdmiEventCallback = SAMPLE_VIO_HdmiEventUpload;
    stHdmiCallbackFun.pPrivateData = HI_NULL;

    CHECK_RET(HI_MAPI_HDMI_Init(enHdmi, &stHdmiCallbackFun));

    printf("Press Enter key to stop preview...\n");
    (void)getchar();

exit:

    if (g_bHdmiEventProFlg) {
        g_bHdmiEventProFlg = HI_FALSE;
        pthread_join(g_hdmiEventProThd, HI_NULL);
    }

    /* stop hdmi 0 */
    if (g_bHdmiStarted) {
        HI_MAPI_HDMI_Stop(HI_HDMI_ID_0);
        g_bHdmiStarted = HI_FALSE;
    }

    HI_MAPI_HDMI_Deinit(HI_HDMI_ID_0);

    /* stop disp 0 */
    if (g_bDispInited) {
        HI_MAPI_DISP_UnBind_VProc(hVpssHdl0, hVPortHdl0, hDispHdl0, hWndHdl0, HI_FALSE);
        HI_MAPI_DISP_StopWindow(hDispHdl0, hWndHdl0);
        HI_MAPI_DISP_Stop(hDispHdl0);
        HI_MAPI_DISP_Deinit(hDispHdl0);
        g_bDispInited = HI_FALSE;
    }

    /* stop vproc 0 */
    HI_MAPI_VPROC_StopPort(hVpssHdl0, hVPortHdl0);
    HI_MAPI_VPROC_UnBindVCap(hVcapPipe0, hPipeChn0, hVpssHdl0);
    HI_MAPI_VPROC_DeinitVpss(hVpssHdl0);

    /* stop vcap 0 */
    HI_MAPI_VCAP_StopISP(hVcapPipe0);
    HI_MAPI_VCAP_StopChn(hVcapPipe0, hPipeChn0);
    HI_MAPI_VCAP_StopDev(hVcapDev0);
    HI_MAPI_VCAP_DeinitSensor(hVcapDev0);

    /* stop media */
    SAMPLE_COMM_Deinit();
#endif
    return HI_SUCCESS;
}

HI_S32 SAMPLE_HDMI_Gyro()
{
#ifdef SUPPORT_GYRO
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8 u8SnsCnt = 1;
    IMU_DRIFT aGyroDrift = { 29861, -70412, -26775 };
    IMU_MATRIX aRotationMatrix = { -32768, 0, 0, 0, -32768, 0, 0, 0, 32768 };

    printf("\n----4K@30fps preview in gyro mode----\n\n");

    /**************************start vcap *******************************/
    HI_HANDLE hVcapDev0 = VCAP_DEV_0;
    HI_HANDLE hVcapPipe0 = VCAP_PIPE_0;
    HI_HANDLE hPipeChn0 = PIPE_CHN_0;

    HI_MAPI_VCAP_ATTR_S stVcapAttr;
    HI_MAPI_SENSOR_ATTR_S stVcapSensorAttr;
    SAMPLE_COMM_INITATTR_S stSampleCommAttr;

    /* sensor0 attr */
    CHECK_RET(SAMPLE_COMM_GetSensorCfg(SAMPLE_SENSOR_4K30, &stVcapSensorAttr));

    /* dev0 attr */
    memcpy(&stVcapAttr.stVcapDevAttr, &g_stDev_4K, sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
    stVcapAttr.u32PipeBindNum = 1;
    stVcapAttr.aPipeIdBind[0] = hVcapPipe0;

    /* Pipe0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0], &g_stPipe_4K30FPS, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /* Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0].astPipeChnAttr[hPipeChn0], &g_stChn_4K30FPS,
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    /* VB and media Attr */
    memset(&stSampleCommAttr, 0, sizeof(SAMPLE_COMM_INITATTR_S));
    memset(&stSampleCommAttr.stViVpssMode, 0, sizeof(VI_VPSS_MODE_S));
    stSampleCommAttr.stResolution.u32Width = stVcapSensorAttr.stSize.u32Width;
    stSampleCommAttr.stResolution.u32Height = stVcapSensorAttr.stSize.u32Height;
    stSampleCommAttr.stViVpssMode.aenMode[hVcapPipe0] = VI_OFFLINE_VPSS_OFFLINE;
    stSampleCommAttr.u8SnsCnt = u8SnsCnt;

    /* init VB and media */
    CHECK_RET(SAMPLE_COMM_Init(stSampleCommAttr, VB_MODE_1));

    /* vcap */
    CHECK_RET(HI_MAPI_VCAP_InitSensor(hVcapDev0, &stVcapSensorAttr));
    CHECK_RET(HI_MAPI_VCAP_SetAttr(hVcapDev0, &stVcapAttr));
    CHECK_RET(HI_MAPI_VCAP_StartDev(hVcapDev0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe0, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StartISP(hVcapPipe0));

    /* set DIS */
    HI_MAPI_VCAP_DIS_ATTR_S stDisAttr;
    memset(&stDisAttr, 0x00, sizeof(HI_MAPI_VCAP_DIS_ATTR_S));

    stDisAttr.stLDCV2Attr.stAttr.s32FocalLenX = 3270795;
    stDisAttr.stLDCV2Attr.stAttr.s32FocalLenY = 3256008;
    stDisAttr.stLDCV2Attr.stAttr.s32CoorShiftX = 191950;
    stDisAttr.stLDCV2Attr.stAttr.s32CoorShiftY = 107950;

    stDisAttr.stLDCV2Attr.stAttr.as32SrcCaliRatio[0][0] = 100000;
    stDisAttr.stLDCV2Attr.stAttr.as32SrcCaliRatio[0][1] = 0;
    stDisAttr.stLDCV2Attr.stAttr.as32SrcCaliRatio[0][2] = 0;
    stDisAttr.stLDCV2Attr.stAttr.as32SrcCaliRatio[0][3] = 0;
    stDisAttr.stLDCV2Attr.stAttr.s32SrcJunPt = 8;

    stDisAttr.stLDCV2Attr.stAttr.as32DstCaliRatio[0][0] = 100000;
    stDisAttr.stLDCV2Attr.stAttr.as32DstCaliRatio[0][1] = 0;
    stDisAttr.stLDCV2Attr.stAttr.as32DstCaliRatio[0][2] = 0;
    stDisAttr.stLDCV2Attr.stAttr.as32DstCaliRatio[0][3] = 0;
    stDisAttr.stLDCV2Attr.stAttr.as32DstJunPt[0] = 8;
    stDisAttr.stLDCV2Attr.stAttr.as32DstJunPt[1] = 8;

    stDisAttr.stDisAttr.bGdcBypass = HI_FALSE;
    stDisAttr.stDisAttr.u32MovingSubjectLevel = 0;
    stDisAttr.stDisAttr.s32RollingShutterCoef = 0;
    stDisAttr.stDisAttr.s32Timelag = -30830;
    stDisAttr.stDisAttr.u32ViewAngle = 120;
    stDisAttr.stDisAttr.u32HorizontalLimit = 512;
    stDisAttr.stDisAttr.u32VerticalLimit = 512;
    stDisAttr.stDisAttr.bStillCrop = HI_FALSE;

    stDisAttr.stDISConfig.enPdtType = DIS_PDT_TYPE_DV;
    stDisAttr.stDISConfig.enMode = DIS_MODE_GYRO;
    stDisAttr.stDISConfig.enMotionLevel = DIS_MOTION_LEVEL_NORMAL;
    stDisAttr.stDISConfig.u32CropRatio = 80;
    stDisAttr.stDISConfig.u32BufNum = 5;
    stDisAttr.stDISConfig.u32GyroOutputRange = 25000;
    stDisAttr.stDISConfig.u32GyroDataBitWidth = 15;
    stDisAttr.stDISConfig.bScale = HI_FALSE;

    stDisAttr.stMotionSensorInfo.u32GyroFSR = 1000 * GRADIENT;
    stDisAttr.stMotionSensorInfo.u32AccFSR = 16 * GRADIENT;

    memcpy(stDisAttr.stMotionSensorInfo.aGyroDrift, aGyroDrift, sizeof(IMU_DRIFT));
    memcpy(stDisAttr.stMotionSensorInfo.aRotationMatrix, aRotationMatrix, sizeof(IMU_MATRIX));

    stDisAttr.stMotionSensorInfo.stMFusionAttr.u32DeviceMask = MSENSOR_DEVICE_GYRO | MSENSOR_DEVICE_ACC;
    stDisAttr.stMotionSensorInfo.stMFusionAttr.u32TemperatureMask = MSENSOR_TEMP_GYRO | MSENSOR_TEMP_ACC;

    stDisAttr.stDisAttr.bEnable = HI_TRUE;
    CHECK_RET(HI_MAPI_VCAP_SetChnDISAttr(hVcapPipe0, hPipeChn0, &stDisAttr));

    /**************************start vproc *******************************/
    HI_HANDLE hVpssHdl0 = 0;
    HI_HANDLE hVPortHdl0 = 0;
    HI_BOOL bStitch = HI_FALSE;
    HI_MAPI_VPSS_ATTR_S stVpssAttr;
    HI_MAPI_VPORT_ATTR_S stVPortAttr;

    /* Vpss0 attr */
    memcpy(&stVpssAttr, &g_stVpss_4K30FPS, sizeof(HI_MAPI_VPSS_ATTR_S));

    /* VPort0 attr */
    memcpy(&stVPortAttr, &g_stVport_4K30FPS, sizeof(HI_MAPI_VPORT_ATTR_S));

    CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl0, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl0, hVPortHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl0, hVPortHdl0));

    /**************************start preview *******************************/
    CHECK_RET(SAMPLE_COMM_StartPreview(hVpssHdl0, hVPortHdl0, bStitch));

    printf("Press Enter key to stop preview...\n");
    (void)getchar();

    stDisAttr.stDisAttr.bEnable = HI_FALSE;
    CHECK_RET(HI_MAPI_VCAP_SetChnDISAttr(hVcapPipe0, hPipeChn0, &stDisAttr));

exit:

    /* stop preview */
    SAMPLE_COMM_StopPreview(hVpssHdl0, hVPortHdl0, bStitch);

    /* stop vproc 0 */
    HI_MAPI_VPROC_StopPort(hVpssHdl0, hVPortHdl0);
    HI_MAPI_VPROC_UnBindVCap(hVcapPipe0, hPipeChn0, hVpssHdl0);
    HI_MAPI_VPROC_DeinitVpss(hVpssHdl0);

    /* stop vcap 0 */
    HI_MAPI_VCAP_StopISP(hVcapPipe0);
    HI_MAPI_VCAP_StopChn(hVcapPipe0, hPipeChn0);
    HI_MAPI_VCAP_StopDev(hVcapDev0);
    HI_MAPI_VCAP_DeinitSensor(hVcapDev0);

    /* stop media */
    SAMPLE_COMM_Deinit();

    return HI_SUCCESS;
#else
    printf("Not support gyro mode...\n");
    return HI_SUCCESS;
#endif
}

HI_S32 SAMPLE_HDMI_Ahd()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE  VcapDev0 = VCAP_DEV_0;
    HI_HANDLE  VcapDev1 = VCAP_DEV_1;
    HI_HANDLE  VcapPipe0 = VCAP_PIPE_0;
    HI_HANDLE  PipeChn0 = PIPE_CHN_0;
    HI_HANDLE  VcapPipe1 = 2;
    HI_MAPI_VCAP_ATTR_S  stVcapAttr;
    HI_MAPI_SENSOR_ATTR_S stVcapSensorAttr;
    SAMPLE_COMM_INITATTR_S stSampleCommAttr;

    printf("-----preview with ahd-----\n");

    /* sensor0 attr */
    stVcapSensorAttr.u8SnsMode = 0;
    stVcapSensorAttr.enWdrMode = WDR_MODE_NONE;
    stVcapSensorAttr.stSize.u32Width = 1920;
    stVcapSensorAttr.stSize.u32Height = 1080;

    /*dev0 attr*/
    memcpy(&stVcapAttr.stVcapDevAttr, &g_stDev_1080P, sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
    stVcapAttr.u32PipeBindNum = 1;
    stVcapAttr.aPipeIdBind[0] = VcapPipe0;

    /*Pipe0 attr*/
    memcpy(&stVcapAttr.astVcapPipeAttr[VcapPipe0], &g_stPipe_1080P30FPS, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[VcapPipe0].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /*pipe0 - Chn0 attr*/
    memcpy(&stVcapAttr.astVcapPipeAttr[VcapPipe0].astPipeChnAttr[0], &g_stChn_1080P30FPS, sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    /* VB and media Attr */
    memset(&stSampleCommAttr, 0, sizeof(SAMPLE_COMM_INITATTR_S));
    memset(&stSampleCommAttr.stViVpssMode, 0, sizeof(VI_VPSS_MODE_S));
    stSampleCommAttr.stResolution.u32Width = stVcapSensorAttr.stSize.u32Width;
    stSampleCommAttr.stResolution.u32Height = stVcapSensorAttr.stSize.u32Height;
    stSampleCommAttr.stViVpssMode.aenMode[VcapPipe0] = VI_OFFLINE_VPSS_OFFLINE;
    stSampleCommAttr.u8SnsCnt = 1;

    /* init VB and media */
    CHECK_RET(SAMPLE_COMM_Init(stSampleCommAttr, VB_MODE_1));

    /*start sensor*/
    CHECK_RET(HI_MAPI_VCAP_InitSensor(VcapDev0, &stVcapSensorAttr));
    CHECK_RET(HI_MAPI_VCAP_SetAttr(VcapDev0, &stVcapAttr));
    CHECK_RET(HI_MAPI_VCAP_StartDev(VcapDev0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(VcapPipe0, PipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StartISP(VcapPipe0));

    /*start bt656*/
    stVcapSensorAttr.u8SnsMode = 1; //bt565 1080P@30fps
    CHECK_RET(HI_MAPI_VCAP_InitSensor(VcapDev1, &stVcapSensorAttr));
    memset(&stVcapAttr, 0, sizeof(HI_MAPI_VCAP_ATTR_S));
    stVcapAttr.u32PipeBindNum = 1;
    stVcapAttr.aPipeIdBind[0] = VcapPipe1;
    memcpy(&stVcapAttr.astVcapPipeAttr[VcapPipe1], &g_stPipe_1080P30FPS, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[VcapPipe1].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;
    stVcapAttr.astVcapPipeAttr[VcapPipe1].bIspBypass = HI_TRUE;
    stVcapAttr.astVcapPipeAttr[VcapPipe1].enPipeBypassMode = VI_PIPE_BYPASS_NONE;
    memcpy(&stVcapAttr.astVcapPipeAttr[VcapPipe1].astPipeChnAttr[0], &g_stChn_1080P30FPS, sizeof(HI_MAPI_PIPE_CHN_ATTR_S));
    CHECK_RET(HI_MAPI_VCAP_SetAttr(VcapDev1, &stVcapAttr));
    CHECK_RET(HI_MAPI_VCAP_StartDev(VcapDev1));
    CHECK_RET(HI_MAPI_VCAP_StartChn(VcapPipe1, PipeChn0));

    /**************************start vproc *******************************/
    HI_HANDLE VpssHdl0 = 0;
    HI_HANDLE VPortHdl0 = 0;
    HI_BOOL bStitch = HI_FALSE;
    HI_MAPI_VPSS_ATTR_S stVpssAttr;
    HI_MAPI_VPORT_ATTR_S stVPortAttr;

    /* Vpss0 attr */
    memcpy(&stVpssAttr, &g_stVpss_1080P30FPS, sizeof(HI_MAPI_VPSS_ATTR_S));

    /* VPort0 attr */
    memcpy(&stVPortAttr, &g_stVport_1080P30FPS, sizeof(HI_MAPI_VPORT_ATTR_S));
    CHECK_RET(HI_MAPI_VPROC_InitVpss(VpssHdl0, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(VcapPipe1, PipeChn0, VpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(VpssHdl0, VPortHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(VpssHdl0, VPortHdl0));
    CHECK_RET(SAMPLE_COMM_StartPreview(VpssHdl0, VPortHdl0, HI_FALSE));

    getchar();
    printf("stop bt656 preview \n");
    SAMPLE_COMM_StopPreview(VpssHdl0, VPortHdl0, HI_FALSE);
    /* stop vproc 0 */
    HI_MAPI_VPROC_StopPort(VpssHdl0, VPortHdl0);
    HI_MAPI_VPROC_UnBindVCap(VcapPipe1, PipeChn0, VpssHdl0);
    HI_MAPI_VPROC_DeinitVpss(VpssHdl0);

    printf("start sensor preview \n");
    /* Vpss0 attr */
    memcpy(&stVpssAttr, &g_stVpss_1080P30FPS, sizeof(HI_MAPI_VPSS_ATTR_S));

    /* VPort0 attr */
    memcpy(&stVPortAttr, &g_stVport_1080P30FPS, sizeof(HI_MAPI_VPORT_ATTR_S));

    CHECK_RET(HI_MAPI_VPROC_InitVpss(VpssHdl0, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(VcapPipe0, PipeChn0, VpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(VpssHdl0, VPortHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(VpssHdl0, VPortHdl0));

    CHECK_RET(SAMPLE_COMM_StartPreview(VpssHdl0, VPortHdl0, HI_FALSE));
    getchar();

exit:
    SAMPLE_COMM_StopPreview(VpssHdl0, VPortHdl0, HI_FALSE);

    /* stop vproc 0 */
    HI_MAPI_VPROC_StopPort(VpssHdl0, VPortHdl0);
    HI_MAPI_VPROC_UnBindVCap(VcapPipe0, PipeChn0, VpssHdl0);
    HI_MAPI_VPROC_DeinitVpss(VpssHdl0);

    /*stop sensor*/
    HI_MAPI_VCAP_StopISP(VcapPipe0);
    HI_MAPI_VCAP_StopChn(VcapPipe0, PipeChn0);
    HI_MAPI_VCAP_StopDev(VcapDev0);
    HI_MAPI_VCAP_DeinitSensor(VcapDev0);


    /*stop bt656*/
    HI_MAPI_VCAP_StopChn(VcapPipe1, PipeChn0);
    HI_MAPI_VCAP_StopDev(VcapDev1);
    HI_MAPI_VCAP_DeinitSensor(VcapDev1);

    /* stop media */
    SAMPLE_COMM_Deinit();

    return HI_SUCCESS;
}

#ifdef __HuaweiLite__
int app_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    HI_S32 s32Ret;

#ifdef __DualSys__
    s32Ret = HI_MAPI_Sys_Init();

    if (s32Ret != HI_SUCCESS) {
        printf("HI_MAPI_Sys_Init fail s32Ret:%x\n", s32Ret);
        return HI_FAILURE;
    }

#endif

    SAMPLE_VIO_Usage();
    g_sampleId = (char)getchar();
    (void)getchar();

    switch (g_sampleId) {
        case '0':
            s32Ret = SAMPLE_VIO_HDMI_Preview();
            break;

#ifdef SUPPORT_HDMI
        case '1':
            s32Ret = SAMPLE_HDMI_AudioOutput();
            break;

        case '2':
            s32Ret = SAMPLE_HDMI_AutoAdapt();
            break;
#endif

#ifdef SUPPORT_GYRO
        case '3':
            s32Ret = SAMPLE_HDMI_Gyro();
            break;
#endif
        case '4':
            s32Ret = SAMPLE_HDMI_Ahd();
            break;
        default:
            printf("the index is invaild!\n");
            SAMPLE_VIO_Usage();
            return HI_FAILURE;
    }

#ifdef __DualSys__
    s32Ret = HI_MAPI_Sys_Deinit();
#endif

    if (s32Ret == HI_SUCCESS) {
        printf("\n The program exited successfully!\n");
    } else {
        printf("\n The program exits abnormally!\n");
    }

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
